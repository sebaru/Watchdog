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
 gchar *Admin_command( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[80];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'AUDIO'" );
       response = Admin_write ( response, "  dbcfg ...             - Get/Set Database Parameters" );
       response = Admin_write ( response, "  tell_mp3 $num         - Send message num with mp3 format" );
       response = Admin_write ( response, "  tell_google $text     - Send $text with google_speech format" );
       response = Admin_write ( response, "  help                  - This help" );
     } else
    if ( ! strcmp ( commande, "tell_mp3" ) )
     { struct CMD_TYPE_MESSAGE msg;
       sscanf ( ligne, "%s %d", commande, &msg.num );                                    /* Découpage de la ligne de commande */
       Jouer_mp3 ( &msg );
       g_snprintf( chaine, sizeof(chaine), " Message id %d sent with mp3", msg.num );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "tell_google" ) )
     { Jouer_google_speech ( ligne + 12 );
       g_snprintf( chaine, sizeof(chaine), " Message sent with google_speech" );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "dbcfg" ) )                     /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { gboolean retour;
       response =  Admin_dbcfg_thread ( response, NOM_THREAD, ligne+6 );                        /* Si changement de parametre */
       retour = Audio_Lire_config();
       g_snprintf( chaine, sizeof(chaine), " Reloading Audio Thread Parameters from Database -> %s",
                   (retour ? "Success" : "Failed") );
       response = Admin_write ( response, chaine );
     } else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
   return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
