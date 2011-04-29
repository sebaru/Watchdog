/**********************************************************************************************************/
/* Watchdogd/Admin/admin_process.c        Gestion des connexions Admin PROCESS au serveur watchdog        */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_process.c
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
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_process ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { gchar thread[128], chaine[128];
       sscanf ( ligne, "%s %s", commande, thread );

       g_snprintf( chaine, sizeof(chaine), " Starting %s\n", thread );
       Write_admin ( client->connexion, chaine );

       if ( ! strcmp ( thread, "arch" ) )
        { if (!Demarrer_arch())                                            /* Demarrage gestion Archivage */
           { Info( Config.log, DEBUG_ADMIN, "Admin: Pb ARCH -> Arret" ); }
        } else
       if ( ! strcmp ( thread, "rs485" ) )
        { if (!Demarrer_rs485())                                        /* Demarrage gestion module RS485 */
           { Info( Config.log, DEBUG_ADMIN, "Admin: Pb RS485 -> Arret" ); }
        } else
       if ( ! strcmp ( thread, "modbus" ) )
        { if (!Demarrer_modbus())                                      /* Demarrage gestion module MODBUS */
           { Info( Config.log, DEBUG_ADMIN, "Admin: Pb MODBUS -> Arret" ); }
        } else
       if ( ! strcmp ( thread, "sms" ) )
        { if (!Demarrer_sms())                                                        /* Démarrage S.M.S. */
           { Info( Config.log, DEBUG_ADMIN, "Admin: Pb SMS -> Arret" ); }
        } else
       if ( ! strcmp ( thread, "audio" ) )
        { if (!Demarrer_audio())                                                   /* Démarrage A.U.D.I.O */
           { Info( Config.log, DEBUG_ADMIN, "Admin: Pb AUDIO -> Arret" ); }
        } else
       if ( ! strcmp ( thread, "dls" ) )
        { if (!Demarrer_dls())                                                        /* Démarrage D.L.S. */
           { Info( Config.log, DEBUG_ADMIN, "Admin: Pb DLS -> Arret" ); }
        }  else
       if ( ! strcmp ( thread, "onduleur" ) )
        { if (!Demarrer_onduleur())                                                 /* Démarrage ONDULEUR */
           { Info( Config.log, DEBUG_ADMIN, "Admin: Pb ONDULEUR -> Arret" ); }
        }  else
       if ( ! strcmp ( thread, "tellstick" ) )
        { if (!Demarrer_tellstick())                                               /* Démarrage TELLSTICK */
           { Info( Config.log, DEBUG_ADMIN, "Admin: Pb TELLSTICK -> Arret" ); }
        }  else
       if ( ! strcmp ( thread, "lirc" ) )
        { if (!Demarrer_lirc())                                                         /* Démarrage LIRC */
           { Info( Config.log, DEBUG_ADMIN, "Admin: Pb LIRC -> Arret" ); }
        } 

     } else
    if ( ! strcmp ( commande, "stop" ) )
     { gchar thread[128], chaine[128];
       sscanf ( ligne, "%s %s", commande, thread );

       g_snprintf( chaine, sizeof(chaine), " Stopping %s\n", thread );
       Write_admin ( client->connexion, chaine );

       if ( ! strcmp ( thread, "all" ) )
        { Stopper_fils(FALSE);                           /* Termine tous les process sauf le thread ADMIN */
        } else
       if ( ! strcmp ( thread, "arch"      ) ) { Partage->com_arch.Thread_run      = FALSE; } else
       if ( ! strcmp ( thread, "rs485"     ) ) { Partage->com_rs485.Thread_run     = FALSE; } else
       if ( ! strcmp ( thread, "modbus"    ) ) { Partage->com_modbus.Thread_run    = FALSE; } else
       if ( ! strcmp ( thread, "sms"       ) ) { Partage->com_sms.Thread_run       = FALSE; } else
       if ( ! strcmp ( thread, "audio"     ) ) { Partage->com_audio.Thread_run     = FALSE; } else
       if ( ! strcmp ( thread, "dls"       ) ) { Partage->com_dls.Thread_run       = FALSE; } else
       if ( ! strcmp ( thread, "onduleur"  ) ) { Partage->com_onduleur.Thread_run  = FALSE; } else
       if ( ! strcmp ( thread, "tellstick" ) ) { Partage->com_tellstick.Thread_run = FALSE; } else
       if ( ! strcmp ( thread, "lirc"      ) ) { Partage->com_lirc.Thread_run      = FALSE; } 

     } else
    if ( ! strcmp ( commande, "list" ) )
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Partage->top = %d\n", Partage->top );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Built-in D.L.S    -> ------------- running = %s\n",
                   (Partage->com_dls.TID       ? "YES" : "NO")
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Built-in SMS      -> ------------- running = %s\n",
                   (Partage->com_sms.TID       ? "YES" : "NO")
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Built-in RS485    -> ------------- running = %s\n",
                   (Partage->com_rs485.TID     ? "YES" : "NO")
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Built-in MODBUS   -> ------------- running = %s\n",
                   (Partage->com_modbus.TID     ? "YES" : "NO")
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Built-in AUDIO    -> ------------- running = %s\n",
                   (Partage->com_audio.TID     ? "YES" : "NO")
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Built-in ONDULEUR -> ------------- running = %s\n",
                   (Partage->com_onduleur.TID     ? "YES" : "NO")
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Built-in ARCHIVE  -> ------------- running = %s\n",
                   (Partage->com_arch.TID       ? "YES" : "NO")
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Library LIRC      -> loaded = %s, running = %s\n",
                   (Partage->com_lirc.dl_handle ? "YES" : "NO"),
                   (Partage->com_lirc.TID       ? "YES" : "NO")
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Library TELLSTICK -> loaded = %s, running = %s\n",
                   (Partage->com_tellstick.dl_handle ? "YES" : "NO"),
                   (Partage->com_tellstick.TID       ? "YES" : "NO")
                 );
       Write_admin ( client->connexion, chaine );

     } else
    if ( ! strcmp ( commande, "SHUTDOWN" ) )
     { Info( Config.log, DEBUG_INFO, "Admin_process : SHUTDOWN demandé" );
       Write_admin ( client->connexion, "SHUTDOWN in progress\n" );
       Partage->com_msrv.Thread_run = FALSE;
     } else
    if ( ! strcmp ( commande, "REBOOT" ) )
     { Info( Config.log, DEBUG_INFO, "Admin_process : REBOOT demandé" );
       Write_admin ( client->connexion, "REBOOT in progress\n" );
       Partage->com_msrv.Thread_reboot = TRUE;
       Partage->com_msrv.Thread_run = FALSE;
     } else
    if ( ! strcmp ( commande, "CLEAR-REBOOT" ) )
     { Info( Config.log, DEBUG_INFO, "Admin_process : CLEAR-REBOOT demandé" );
       Write_admin ( client->connexion, "CLEAR-REBOOT in progress\n" );
       Partage->com_msrv.Thread_clear_reboot = TRUE;
       Partage->com_msrv.Thread_reboot = TRUE;
       Partage->com_msrv.Thread_run = FALSE;
     } else
    if ( ! strcmp ( commande, "RELOAD" ) )
     { Info( Config.log, DEBUG_INFO, "Admin_process : RELOAD demandé" );
       Write_admin ( client->connexion, "RELOAD in progress\n" );
       Partage->com_msrv.Thread_reload = TRUE;
     } else
    if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'PROCESS'\n" );
       Write_admin ( client->connexion,
                     "  start thread         - Start a thread (arch,rs485,modbus,sms,audio,dls,onduleur,tellstick)\n" );
       Write_admin ( client->connexion,
                     "  stop                 - Stop thread (all,arch,rs485,modbus,sms,audio,dls,onduleur,tellstick)\n" );
       Write_admin ( client->connexion,
                     "  RELOAD               - Reload configuration\n" );
       Write_admin ( client->connexion,
                     "  REBOOT               - Restart all processes\n" );
       Write_admin ( client->connexion,
                     "  CLEAR-REBOOT         - Restart all processes with no DATA import/export\n" );
       Write_admin ( client->connexion,
                     "  SHUTDOWN             - Stop processes\n" );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
