/******************************************************************************************************************************/
/* Watchdogd/Imsg/admin_imsg.c        Gestion des connexions Admin IMSG au serveur watchdog                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                                      sam. 28 juil. 2012 16:35:09 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_imsg.c
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
 #include "Audio.h"

/******************************************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                                     */
/* Entrée: Le connexion d'admin, la ligne a traiter                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Admin_command( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[80];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'AUDIO'\n" );
       Admin_write ( connexion, "  dbcfg ...             - Get/Set Database Parameters\n" );
       Admin_write ( connexion, "  tell_mp3 $num         - Send message num with mp3 format\n" );
       Admin_write ( connexion, "  tell_google $text     - Send $text with google_speech format\n" );
       Admin_write ( connexion, "  help                  - This help\n" );
     } else
    if ( ! strcmp ( commande, "tell_mp3" ) )
     { struct CMD_TYPE_MESSAGE msg;
       sscanf ( ligne, "%s %d", commande, &msg.num );                                    /* Découpage de la ligne de commande */
       Jouer_mp3 ( &msg );
       g_snprintf( chaine, sizeof(chaine), " Message id %d sent with mp3\n", msg.num );
       Admin_write ( connexion, chaine );
     } else
    if ( ! strcmp ( commande, "tell_google" ) )
     { Jouer_google_speech ( ligne + 12 );
       g_snprintf( chaine, sizeof(chaine), " Message sent with google_speech\n" );
       Admin_write ( connexion, chaine );
     } else
    if ( ! strcmp ( commande, "dbcfg" ) )                     /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { if (Admin_dbcfg_thread ( connexion, NOM_THREAD, ligne+6 ) == TRUE)                       /* Si changement de parametre */
        { gboolean retour;
          retour = Audio_Lire_config();
          g_snprintf( chaine, sizeof(chaine), " Reloading Audio Thread Parameters from Database -> %s\n",
                      (retour ? "Success" : "Failed") );
          Admin_write ( connexion, chaine );
        }
     } else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
