/**********************************************************************************************************/
/* Watchdogd/Tellstick/admin_tellstick.c        Gestion des connexions Admin IMSG au serveur watchdog     */
/* Projet WatchDog version 2.0       Gestion d'habitat                  sam. 28 juil. 2012 16:35:09 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_tellstick.c
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
 #include "Tellstick.h"

/**********************************************************************************************************/
/* Admin_tellstick_learn: Envoi une commande de LEARN tellstick                                           */
/* Entrée: Le client admin et le numéro ID du tellstick                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_tellstick_learn ( struct CLIENT *client, gint id )
  { int methods;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Envoi de la commande LEARN Tellstick\n" );
    Admin_write ( client, chaine );

    methods = tdMethods( id, TELLSTICK_LEARN );                                 /* Get methods of device */

    if ( methods | TELLSTICK_LEARN )
     { Info_new( Config.log, Cfg_tellstick.lib->Thread_debug, LOG_INFO, "Run_tellstick: Learning %d", id );
       tdLearn ( id );
     }

    g_snprintf( chaine, sizeof(chaine), "   Tellstick -> Learning of device = %d\n", id );
    Admin_write ( client, chaine );
  }
/**********************************************************************************************************/
/* Admin_tellstick_on: Envoi une commande de START tellstick                                              */
/* Entrée: Le client admin et le numéro ID du tellstick                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_tellstick_on ( struct CLIENT *client, gint id )
  { int methods;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Demande d'activation d'un device Tellstick\n" );
    Admin_write ( client, chaine );

    methods = tdMethods( id, TELLSTICK_TURNON );                                /* Get methods of device */

    if ( methods | TELLSTICK_TURNON )
     { Info_new( Config.log, Cfg_tellstick.lib->Thread_debug, LOG_INFO, "Run_tellstick: Starting %d", id );
       tdTurnOn ( id );
     }

    g_snprintf( chaine, sizeof(chaine), "   Tellstick -> Starting device = %d\n", id );
    Admin_write ( client, chaine );
  }
/**********************************************************************************************************/
/* Admin_tellstick_off : Envoi une commande de STOP  tellstick                                            */
/* Entrée: Le client admin et le numéro ID du tellstick                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_tellstick_off ( struct CLIENT *client, gint id )
  { int methods;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Demande de desactivation d'un deviece Tellstick\n" );
    Admin_write ( client, chaine );

    methods = tdMethods( id, TELLSTICK_TURNOFF );                               /* Get methods of device */

    if ( methods | TELLSTICK_TURNOFF )
     { Info_new( Config.log, Cfg_tellstick.lib->Thread_debug, LOG_INFO, "Run_tellstick: Stopping %d", id );
       tdTurnOff ( id );
     }

    g_snprintf( chaine, sizeof(chaine), "   Tellstick -> Stoppping device = %d\n", id );
    Admin_write ( client, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_tellstick_list ( struct CLIENT *client )
  { int nbrDevice, i, supportedMethods, methods;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des device Tellstick\n" );
    Admin_write ( client, chaine );

    nbrDevice = tdGetNumberOfDevices();
    g_snprintf( chaine, sizeof(chaine), "   Tellstick -> Number of devices = %d\n", nbrDevice );
    Admin_write ( client, chaine );

    for (i= 0; i<nbrDevice; i++)
     { char *name, *proto, *house, *unit;
       int id;
       id    = tdGetDeviceId( i );
       name  = tdGetName( id );
       proto = tdGetProtocol( id );
       house = tdGetDeviceParameter( id, "house", "NULL" );
       unit  = tdGetDeviceParameter( id, "unit", "NULL" );
       supportedMethods = TELLSTICK_TURNON | TELLSTICK_TURNOFF | TELLSTICK_BELL | TELLSTICK_LEARN;
       methods = tdMethods( id, supportedMethods );

       g_snprintf( chaine, sizeof(chaine),
                   "   Tellstick [%d] -> proto=%s, house=%s, unit=%s, methods=%s-%s-%s-%s, name=%s\n",
                   id, proto, house, unit,
                   ( methods & TELLSTICK_TURNON  ? "ON"    : "  "     ),
                   ( methods & TELLSTICK_TURNOFF ? "OFF"   : "   "    ),
                   ( methods & TELLSTICK_BELL    ? "BELL"  : "    "   ),
                   ( methods & TELLSTICK_LEARN   ? "LEARN" : "      " ),
                   name
                 );
       Admin_write ( client, chaine );
       tdReleaseString(name);
       tdReleaseString(proto);
       tdReleaseString(house);
       tdReleaseString(unit);
     }
  }
/**********************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                 */
/* Entrée: Le client d'admin, la ligne a traiter                                                          */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command( struct CLIENT *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "learn" ) )
     { gint id;
       sscanf ( ligne, "%s %d", commande, &id );                     /* Découpage de la ligne de commande */
       Admin_tellstick_learn ( client, id );
     }
    else if ( ! strcmp ( commande, "on" ) )
     { gint id;
       sscanf ( ligne, "%s %d", commande, &id );                     /* Découpage de la ligne de commande */
       Admin_tellstick_on ( client, id );
     }
    else if ( ! strcmp ( commande, "off" ) )
     { gint id;
       sscanf ( ligne, "%s %d", commande, &id );                     /* Découpage de la ligne de commande */
       Admin_tellstick_off ( client, id );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_tellstick_list ( client );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( client, "  -- Watchdog ADMIN -- Help du mode 'Tellstick'\n" );
       Admin_write ( client, "  on id             - Start telltick ID\n" );
       Admin_write ( client, "  off id            - Stop telltick ID\n" );
       Admin_write ( client, "  learn id          - Learn telltick ID\n" );
       Admin_write ( client, "  list              - List tellstick id and infos\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown Tellstick command : %s\n", ligne );
       Admin_write ( client, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
