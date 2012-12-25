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
 
 #include <unistd.h>
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Admin_process: Appeller lorsque l'admin envoie une commande 'process' dans la ligne de commande        */
/* Entrée: La connexion cliente et la ligne de commande, et le buffer de sortie                           */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_process ( struct CLIENT *client, gchar *buffer, gchar *ligne )
  { struct LIBRAIRIE *lib;
    gchar commande[128];
    GSList *liste;

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { gchar thread[128], chaine[128];
       guint num;
       sscanf ( ligne, "%s %s %d", commande, thread, &num );

       g_snprintf( chaine, sizeof(chaine), " Trying to start %s\n", thread );
       g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );

       if ( ! strcmp ( thread, "arch" ) )
        { if (!Demarrer_arch())                                            /* Demarrage gestion Archivage */
           { Info_new( Config.log, Config.log_all, LOG_INFO, "Admin: Pb ARCH -> Arret" ); }
        } else
       if ( ! strcmp ( thread, "dls" ) )
        { if (!Demarrer_dls())                                                        /* Démarrage D.L.S. */
           { Info_new( Config.log, Config.log_all, LOG_INFO, "Admin: Pb DLS -> Arret" ); }
        }  else
       if ( ! strcmp ( thread, "ssrv" ) )
        { if (num<0 || num>=Config.max_serveur)
           { g_snprintf( chaine, sizeof(chaine), " num %d out of range\n", num );
             g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
           }
          else if (!Demarrer_sous_serveur(num))                                    /* Démarrage d'un SSRV */
           { Info_new( Config.log, Config.log_all, LOG_INFO, "Admin: Pb SSRV -> Arret" ); }
          else Gerer_jeton();                              /* Affectation du jeton a un des sous-serveurs */
        }
       else
        { GSList *liste;
          liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { struct LIBRAIRIE *lib;
             lib = (struct LIBRAIRIE *)liste->data;
             if ( ! strcmp( lib->admin_prompt, thread ) )
              { if (Start_librairie(lib))
                 { g_snprintf( chaine, sizeof(chaine), " Library %s started\n", lib->admin_prompt ); }
                else
                 { g_snprintf( chaine, sizeof(chaine), " Error while starting library %s\n", lib->admin_prompt ); }
                   g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
              }
             liste = liste->next;
           }
        }
     } else
    if ( ! strcmp ( commande, "load" ) )
     { gchar thread[128], chaine[128];
       struct LIBRAIRIE *lib;
       sscanf ( ligne, "%s %s", commande, thread );
       g_snprintf( chaine, sizeof(chaine), "libwatchdog-server-%s.so", thread );
       if ( (lib = Charger_librairie_par_fichier( NULL, chaine )) )/* Chargement de la librairie dynamique */
        { g_snprintf( chaine, sizeof(chaine), " Library %s loaded\n", thread );
          if (Start_librairie(lib))
           { g_snprintf( chaine, sizeof(chaine), " Library %s started\n", thread ); }
           else
           { g_snprintf( chaine, sizeof(chaine), " Error while starting library %s\n", thread ); }
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error while loading library %s\n", thread ); }
       g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
     } else
    if ( ! strcmp ( commande, "unload" ) )
     { gchar thread[128], chaine[128];
       sscanf ( ligne, "%s %s", commande, thread );
       if (Decharger_librairie_par_prompt( thread ))               /* Déchargement de la librairie dynamique */
        { g_snprintf( chaine, sizeof(chaine), " Library %s stopped and unloaded\n", thread ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error while unloading library %s\n", thread ); }
       g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
     } else
    if ( ! strcmp ( commande, "stop" ) )
     { gchar thread[128], chaine[128];
       sscanf ( ligne, "%s %s", commande, thread );

       g_snprintf( chaine, sizeof(chaine), " Trying to stop %s\n", thread );
       g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );

       if ( ! strcmp ( thread, "all" ) )
        { Stopper_fils(FALSE);                           /* Termine tous les process sauf le thread ADMIN */
        } else
       if ( ! strcmp ( thread, "arch"      ) ) { Partage->com_arch.Thread_run      = FALSE; } else
       if ( ! strcmp ( thread, "dls"       ) ) { Partage->com_dls.Thread_run       = FALSE; }
       else
        { GSList *liste;
          liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { struct LIBRAIRIE *lib;
             lib = (struct LIBRAIRIE *)liste->data;
             if ( ! strcmp( lib->admin_prompt, thread ) )
              { if (Stop_librairie(lib))
                 { g_snprintf( chaine, sizeof(chaine), " Library %s (%s) stopped\n",
                               lib->admin_prompt, lib->nom_fichier );
                 }
                else
                 { g_snprintf( chaine, sizeof(chaine), " Error while stopping library %s (%s) \n",
                               lib->admin_prompt, lib->nom_fichier );
                 }
                g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
              }
             liste = liste->next;
           }
        }

     } else
    if ( ! strcmp ( commande, "list" ) )
     { gchar chaine[128];
       guint i;

       g_snprintf( chaine, sizeof(chaine), " -- Liste des process\n" );
       g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );

       g_snprintf( chaine, sizeof(chaine), " Partage->top = %d\n", Partage->top );
       g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );

       for (i=0; i<Config.max_serveur; i++)
        { g_snprintf( chaine, sizeof(chaine), " Built-in SSRV[%d]  -> ------------- running = %s, TID = %d\n", i,
                      (Partage->Sous_serveur[i].Thread_run ? "YES" : " NO"), (gint)Partage->Sous_serveur[i].pid
                    );
          g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
        }

       g_snprintf( chaine, sizeof(chaine), " Built-in D.L.S    -> ------------- running = %s, TID = %d\n",
                   (Partage->com_dls.Thread_run ? "YES" : " NO"), (gint)Partage->com_dls.TID
                 );
       g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );

       g_snprintf( chaine, sizeof(chaine), " Built-in ARCHIVE  -> ------------- running = %s, TID = %d\n",
                   (Partage->com_arch.Thread_run ? "YES" : " NO"), (gint)Partage->com_arch.TID
                 );
       g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );

       liste = Partage->com_msrv.Librairies;                           /* Parcours de toutes les librairies */
       while(liste)
        { gchar result[256];
          lib = (struct LIBRAIRIE *)liste->data;
          memset ( result, ' ', sizeof(result) );
          memcpy ( result + 0, " Library ", 9 );
          memcpy ( result + 9, lib->admin_prompt, strlen(lib->admin_prompt) );
          if (lib->Thread_run == TRUE)
           { memcpy ( result + 25, "-> running YES, TID = ", 22 ); }
          else
           { memcpy ( result + 25, "-> running  NO, TID = ", 22 ); }
          g_snprintf( chaine, sizeof(chaine), "%d (%s)\n", (gint) lib->TID, lib->nom_fichier );
          memcpy( result + 47, chaine, strlen(chaine) + 1 );   /* +1 pour choper le \0 de fin de chaine ! */
          g_strlcat ( buffer, result, NBR_CARAC_BUFFER_ADMIN );
          liste = liste->next;
        }

     } else
    if ( ! strcmp ( commande, "SHUTDOWN" ) )
     { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Admin_process : SHUTDOWN demandé" );
       g_strlcat ( buffer, "SHUTDOWN in progress\n", NBR_CARAC_BUFFER_ADMIN );
       SB( 7, TRUE );                                                     /* Message audio avant Shutdown */
       sleep(1);
       Partage->com_msrv.Thread_run = FALSE;
       SB( 7, FALSE );                                                    /* Message audio avant Shutdown */
     } else
    if ( ! strcmp ( commande, "REBOOT" ) )
     { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Admin_process : REBOOT demandé" );
       g_strlcat ( buffer, "REBOOT in progress\n", NBR_CARAC_BUFFER_ADMIN );
       SB( 8, TRUE );                                                       /* Message audio avant Reboot */
       sleep(1);
       Partage->com_msrv.Thread_reboot = TRUE;
       Partage->com_msrv.Thread_run = FALSE;
       SB( 8, FALSE );                                                      /* Message audio avant Reboot */
     } else
    if ( ! strcmp ( commande, "CLEAR-REBOOT" ) )
     { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Admin_process : CLEAR-REBOOT demandé" );
       g_strlcat ( buffer, "CLEAR-REBOOT in progress\n", NBR_CARAC_BUFFER_ADMIN );
       SB( 8, TRUE );                                                       /* Message audio avant Reboot */
       sleep(1);
       Partage->com_msrv.Thread_clear_reboot = TRUE;
       Partage->com_msrv.Thread_reboot = TRUE;
       Partage->com_msrv.Thread_run = FALSE;
       SB( 8, FALSE );                                                      /* Message audio avant Reboot */
     } else
    if ( ! strcmp ( commande, "RELOAD" ) )
     { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Admin_process : RELOAD demandé" );
       g_strlcat ( buffer, "RELOAD in progress\n", NBR_CARAC_BUFFER_ADMIN );
       Partage->com_msrv.Thread_reload = TRUE;
     } else
    if ( ! strcmp ( commande, "help" ) )
     { g_strlcat ( buffer, "  -- Watchdog ADMIN -- Help du mode 'PROCESS'\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  load thread          - Load a library (but not start it !)\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  unload thread        - Unload a library\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  start thread         - Start a thread (arch,modbus,dls,ssrv num, or library name)\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  stop                 - Stop thread (all,arch,modbus,dls,ssrv num, or library name)\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  list                 - Liste les statut des threads\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  RELOAD               - Reload configuration\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  REBOOT               - Restart all processes\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  CLEAR-REBOOT         - Restart all processes with no DATA import/export\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  SHUTDOWN             - Stop processes\n", NBR_CARAC_BUFFER_ADMIN );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown PROCESS command : %s\n", ligne );
       g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
