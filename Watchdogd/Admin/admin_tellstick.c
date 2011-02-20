/**********************************************************************************************************/
/* Watchdogd/Admin/admin_tellstick.c        Gestion des connexions Admin PROCESS au serveur watchdog      */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 20 févr. 2011 21:29:59 CET */
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
 
 #include <glib.h>
 #include <telldus-core.h>
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_tellstick_list ( struct CLIENT_ADMIN *client )
  { int nbrDevice, i, supportedMethods, methods;
    gchar chaine[128];

    tdInit();

    nbrDevice = tdGetNumberOfDevices();
    g_snprintf( chaine, sizeof(chaine), "   Tellstick -> Number of devices = %d\n", nbrDevice );
    Write_admin ( client->connexion, chaine );

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
                   "   Tellstick [%d] -> name=%s, proto=%s, house=%s, unit=%s, methods=%s-%s-%s-%s\n",
                   id, name, proto, house, unit,
                   ( methods & TELLSTICK_TURNON  ? "ON"    : "  "     ),
                   ( methods & TELLSTICK_TURNOFF ? "OFF"   : "   "    ),
                   ( methods & TELLSTICK_BELL    ? "BELL"  : "    "   ),
                   ( methods & TELLSTICK_LEARN   ? "LEARN" : "      " )
                 );
       Write_admin ( client->connexion, chaine );
       tdReleaseString(name);
       tdReleaseString(proto);
       tdReleaseString(house);
       tdReleaseString(unit);
     }
    tdClose();
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_tellstick ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     {  /**/

     } else if ( ! strcmp ( commande, "list" ) )
     { Admin_tellstick_list ( client );
     } else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'TELLSTICK'\n" );
       Write_admin ( client->connexion,
                     "  ************         - Start a thread (arch,rs,mbus,sms,audio,dls,onduleur,tellstick)\n" );
       Write_admin ( client->connexion,
                     "  **********           - Stop all thread\n" );
       Write_admin ( client->connexion,
                     "  ************         - Reload configuration\n" );
       Write_admin ( client->connexion,
                     "  ***************      - Restart all tellstickes\n" );
       Write_admin ( client->connexion,
                     "  ************         - Restart all tellstickes with no DATA import/export\n" );
       Write_admin ( client->connexion,
                     "  list                 - list all devices\n" );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
