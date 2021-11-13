/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_process.c        Gestion des responses Admin PROCESS au serveur watchdog                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
 
 #include <unistd.h>
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Admin_start: Demarre un thread en parametre                                                                                */
/* Entrée: La response d'admin et le nom du thread a demarrer                                                                */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_start ( gchar *response, gchar *thread )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " | - Trying to start %s", thread );
    response = Admin_write ( response, chaine );

    if ( ! strcmp ( thread, "archive" ) )
     { if (!Demarrer_arch())                                                                   /* Demarrage gestion Archivage */
        { Info_new( Config.log, Config.log_msrv, LOG_INFO, "Admin: Pb ARCH -> Arret" ); }
       else { g_snprintf( chaine, sizeof(chaine), " | - ARCH started" );
              response = Admin_write ( response, chaine );
            }
     } else
    if ( ! strcmp ( thread, "dls" ) )
     { if (!Demarrer_dls())                                                                               /* Démarrage D.L.S. */
        { Info_new( Config.log, Config.log_msrv, LOG_INFO, "Admin: Pb DLS -> Arret" ); }
       else { g_snprintf( chaine, sizeof(chaine), " | - D.L.S started" );
              response = Admin_write ( response, chaine );
            }
     }
    else
     { GSList *liste;
       gint found;
       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       found = 0;
       while(liste)
        { struct LIBRAIRIE *lib;
          lib = (struct LIBRAIRIE *)liste->data;
          if ( ! strcmp( lib->admin_prompt, thread ) )
           { if (Start_librairie(lib))
              { g_snprintf( chaine, sizeof(chaine), " | - Library %s started", lib->admin_prompt );
                found++;
              }
             else
              { g_snprintf( chaine, sizeof(chaine), " | - Error while starting library %s", lib->admin_prompt ); }
             response = Admin_write ( response, chaine );
           }
          liste = liste->next;
        }
       g_snprintf( chaine, sizeof(chaine), " | - Number of librairie(s) started : %d", found );
       response = Admin_write ( response, chaine );
     }
   return(response);
  }
/******************************************************************************************************************************/
/* Admin_stop: Arrete un thread en parametre                                                                                  */
/* Entrée: La response d'admin et le nom du thread                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_stop ( gchar *response, gchar *thread )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), "  | - Trying to stop %s", thread );
    response = Admin_write ( response, chaine );

    if ( ! strcmp ( thread, "all" ) )
     { Stopper_fils(FALSE);                                                  /* Termine tous les process sauf le thread ADMIN */
     } else
    if ( ! strcmp ( thread, "arch" ) ) { Partage->com_arch.Thread_run = FALSE; }
    else
    if ( ! strcmp ( thread, "dls"  ) ) { Partage->com_dls.Thread_run  = FALSE; }
    else
     { GSList *liste;
       gint found;
       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       found = 0;
       while(liste)
        { struct LIBRAIRIE *lib;
          lib = (struct LIBRAIRIE *)liste->data;
          if ( ! strcmp( lib->admin_prompt, thread ) )
           { if (Stop_librairie(lib))
              { g_snprintf( chaine, sizeof(chaine), " | - Library %s (%s) stopped", lib->admin_prompt, lib->nom_fichier );
                found++;
              }
             else
              { g_snprintf( chaine, sizeof(chaine), " | - Error while stopping library %s (%s) ",
                            lib->admin_prompt, lib->nom_fichier );
              }
             response = Admin_write ( response, chaine );
           }
          liste = liste->next;
        }
       g_snprintf( chaine, sizeof(chaine), " | - Number of librairie(s) stopped : %d", found );
       response = Admin_write ( response, chaine );
     }
   return(response);
  }
/******************************************************************************************************************************/
/* Admin_process: Appellée lorsque l'admin envoie une commande 'process' dans la ligne de commande                            */
/* Entrée: La response responsee et la ligne de commande, et le buffer de sortie                                            */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_process ( gchar *response, gchar *ligne )
  { struct LIBRAIRIE *lib;
    gchar commande[128];
    GSList *liste;

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { gchar thread[128];
       guint num;
       sscanf ( ligne, "%s %s %d", commande, thread, &num );
       return(Admin_start ( response, thread ));
     } else
    if ( ! strcmp ( commande, "stop" ) )
     { gchar thread[128];
       sscanf ( ligne, "%s %s", commande, thread );
       return(Admin_stop ( response, thread ));
     } else
    if ( ! strcmp ( commande, "restart" ) )
     { gchar thread[128];
       sscanf ( ligne, "%s %s", commande, thread );
       response = Admin_stop ( response, thread );
       sleep(5);
       response = Admin_start ( response, thread );
     } else
    if ( ! strcmp ( commande, "load" ) )
     { gchar thread[128], chaine[128];
       struct LIBRAIRIE *lib;
       sscanf ( ligne, "%s %s", commande, thread );
       if ( (lib = Charger_librairie_par_prompt( thread )) )      /* Chargement de la librairie dynamique */
        { g_snprintf( chaine, sizeof(chaine), " | - Library %s loaded", thread );
          if (Start_librairie(lib))
           { g_snprintf( chaine, sizeof(chaine), " | - Library %s started", thread ); }
           else
           { g_snprintf( chaine, sizeof(chaine), " | - Error while starting library %s", thread ); }
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " | - Error while loading library %s", thread ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "unload" ) )
     { gchar thread[128], chaine[128];
       sscanf ( ligne, "%s %s", commande, thread );
       if (Decharger_librairie_par_prompt( thread ))               /* Déchargement de la librairie dynamique */
        { g_snprintf( chaine, sizeof(chaine), " | - Library %s stopped and unloaded", thread ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " | - Error while unloading library %s", thread ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "list" ) )
     { gchar chaine[128];

       g_snprintf( chaine, sizeof(chaine), " -- Liste des process" );
       response = Admin_write ( response, chaine );

       g_snprintf( chaine, sizeof(chaine),
                  " | - Built-in D.L.S          -> running = %s, TID = %p",
                   (Partage->com_dls.Thread_run ? "YES" : " NO"), (void *)Partage->com_dls.TID
                 );
       response = Admin_write ( response, chaine );

       g_snprintf( chaine, sizeof(chaine),
                  " | - Built-in ARCHIVE        -> running = %s, TID = %p",
                   (Partage->com_arch.Thread_run ? "YES" : " NO"), (void *)Partage->com_arch.TID
                 );
       response = Admin_write ( response, chaine );

       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       while(liste)
        { gchar result[256];
          lib = (struct LIBRAIRIE *)liste->data;
          memset ( result, ' ', sizeof(result) );
          memcpy ( result + 0, " | - Library ", 13 );
          memcpy ( result + 13, lib->admin_prompt, strlen(lib->admin_prompt) );
          if (lib->Thread_run == TRUE)
           { memcpy ( result + 29, "-> running YES, TID = ", 22 ); }
          else
           { memcpy ( result + 29, "-> running  NO, TID = ", 22 ); }
          g_snprintf( chaine, sizeof(chaine), "%p (%s)", (void *)lib->TID, lib->nom_fichier );
          memcpy( result + 51, chaine, strlen(chaine) + 1 );                       /* +1 pour choper le \0 de fin de chaine ! */
          response = Admin_write ( response, result );
          liste = liste->next;
        }

     } else
    if ( ! strcmp ( commande, "SHUTDOWN" ) )
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Admin_process : SHUTDOWN demandé" );
       SB_SYS( 7, TRUE );                                                                     /* Message audio avant Shutdown */
       sleep(5);
       Partage->com_msrv.Thread_run = FALSE;
       SB_SYS( 7, FALSE );                                                                    /* Message audio avant Shutdown */
       response = Admin_write ( response, " | - SHUTDOWN in progress" );
     } else
    if ( ! strcmp ( commande, "REBOOT" ) )
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Admin_process : REBOOT demandé" );
       SB_SYS( 8, TRUE );                                                                       /* Message audio avant Reboot */
       sleep(5);
       Partage->com_msrv.Thread_reboot = TRUE;
       Partage->com_msrv.Thread_run = FALSE;
       SB_SYS( 8, FALSE );                                                                      /* Message audio avant Reboot */
       response = Admin_write ( response, " | - REBOOT in progress" );
     } else
    if ( ! strcmp ( commande, "CLEAR-REBOOT" ) )
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Admin_process : CLEAR-REBOOT demandé" );
       SB_SYS( 8, TRUE );                                                                       /* Message audio avant Reboot */
       sleep(5);
       Partage->com_msrv.Thread_clear_reboot = TRUE;
       Partage->com_msrv.Thread_reboot = TRUE;
       Partage->com_msrv.Thread_run = FALSE;
       SB_SYS( 8, FALSE );                                                                      /* Message audio avant Reboot */
       response = Admin_write ( response, " | - CLEAR-REBOOT in progress" );
     } else
    if ( ! strcmp ( commande, "RELOAD" ) )
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Admin_process : RELOAD demandé" );
       Partage->com_msrv.Thread_reload = TRUE;
       response = Admin_write ( response, " | - RELOAD in progress" );
     } else
    if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'PROCESS'" );
       response = Admin_write ( response, "  load $thread         - Load a library (but not start it !)" );
       response = Admin_write ( response, "  unload $thread       - Unload a library" );
       response = Admin_write ( response, "  start $thread        - Start a thread (arch,modbus,dls, or library name)" );
       response = Admin_write ( response, "  stop $thread         - Stop thread (all,arch,modbus,dls, or library name)" );
       response = Admin_write ( response, "  restart $thread      - Stop & Start thread (arch,modbus,dls, or library name)" );
       response = Admin_write ( response, "  list                 - Liste les statut des threads" );
       response = Admin_write ( response, "  RELOAD               - Reload configuration" );
       response = Admin_write ( response, "  REBOOT               - Restart all processes" );
       response = Admin_write ( response, "  CLEAR-REBOOT         - Restart all processes with no DATA import/export" );
       response = Admin_write ( response, "  SHUTDOWN             - Stop processes" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " | - Unknown PROCESS command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
